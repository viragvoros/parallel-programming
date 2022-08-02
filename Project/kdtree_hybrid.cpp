#include <mpi.h>
#include <omp.h>
#include <algorithm>
#include <iostream>
#include <functional>
#include <chrono>
#include <random>
#include <math.h>

#include "Utility.hpp"

#define DEBUG 0


/***************************************************************************************/
float Point::distance_squared(Point &a, Point &b){
    if(a.dimension != b.dimension){
        std::cout << "Dimensions do not match!" << std::endl;
        exit(1);
    }
    float dist = 0;
    for(int i = 0; i < a.dimension; ++i){
        float tmp = a.coordinates[i] - b.coordinates[i];
        dist += tmp * tmp;
    }
    return dist;
}
/***************************************************************************************/


/***************************************************************************************/
Node* build_tree_rec(Point** point_list, int num_points, int depth){
    if (num_points <= 0){
        return nullptr;
    }

    if (num_points == 1){
        return new Node(point_list[0], nullptr, nullptr); 
    }

    int dim = point_list[0]->dimension;

    // sort list of points based on axis
    int axis = depth % dim;
    using std::placeholders::_1;
    using std::placeholders::_2;

    std::sort(
        point_list, point_list + (num_points - 1), 
        std::bind(Point::compare, _1, _2, axis));

    // select median
    Point** median = point_list + (num_points / 2);
    Point** left_points = point_list;
    Point** right_points = median + 1;

    int num_points_left = num_points / 2;
    int num_points_right = num_points - (num_points / 2) - 1; 

    // defining left and right nodes before task distribution
    Node* left_node;
    Node* right_node;

    // left subtree task
    #pragma omp task shared(left_node) final(depth > 1)
    left_node = build_tree_rec(left_points, num_points_left, depth + 1);
    
    // right subtree task
    #pragma omp task shared(right_node) final(depth > 1)
    right_node = build_tree_rec(right_points, num_points_right, depth + 1);

    // return median node after all tasks finished
    #pragma omp taskwait
    return new Node(*median, left_node, right_node);  
}

Node* build_tree(Point** point_list, int num_nodes){
    Node* tree;

    // starting the parallel code
    #pragma omp parallel
    #pragma omp single 
    tree = build_tree_rec(point_list, num_nodes, 0);
    
    return tree;
}
/***************************************************************************************/


/***************************************************************************************/
Node* nearest(Node* root, Point* query, int depth, Node* best, float &best_dist) {
    // leaf node
    if (root == nullptr){
        return nullptr; 
    }

    int dim = query->dimension;
    int axis = depth % dim;

    Node* best_local = best;
    float best_dist_local = best_dist;
    
    float d_euclidian = root->point->distance_squared(*query);
    float d_axis = query->coordinates[axis] - root->point->coordinates[axis];
    float d_axis_squared = d_axis * d_axis;

    if (d_euclidian < best_dist_local){
        best_local = root;
        best_dist_local = d_euclidian;
    }

    Node* visit_branch;
    Node* other_branch;

    if(d_axis < 0){
        // query point is smaller than root node in axis dimension, i.e. go left
        visit_branch = root->left;
        other_branch = root->right;
    } else{
        // query point is larger than root node in axis dimension, i.e. go right
        visit_branch = root->right;
        other_branch = root->left;
    }

    Node* further = nearest(visit_branch, query, depth + 1, best_local, best_dist_local);
    if (further != nullptr){
        float dist_further = further->point->distance_squared(*query);
        if (dist_further < best_dist_local){
            best_dist_local = dist_further;
            best_local = further;
        }
    }
    
    if (d_axis_squared < best_dist_local) {
        further = nearest(other_branch, query, depth + 1, best_local, best_dist_local);
        if (further != nullptr){
            float dist_further = further->point->distance_squared(*query);
            if (dist_further < best_dist_local){
                // best_dist_local = dist_further;
                best_local = further;
            }
        }
    }
    
    return best_local;
}


Node* nearest_neighbor(Node* root, Point* query){
    float best_dist = root->point->distance_squared(*query);
    return nearest(root, query, 0, root, best_dist);
}

/***************************************************************************************/
int main(int argc, char **argv){
    int rank;
    int size;
    int provided;
    // initializing MPI
    MPI_Init_thread (&argc, &argv, MPI_THREAD_FUNNELED, &provided);
    MPI_Comm_rank (MPI_COMM_WORLD, &rank);
    MPI_Comm_size (MPI_COMM_WORLD, &size);

    int seed = 0;
    int dim = 0;
    int num_points = 0;
    int num_queries = 10;

    if(rank == 0) {
        #if DEBUG
            // for measuring your local runtime
            auto tick = std::chrono::high_resolution_clock::now();
            Utility::specify_problem(argc, argv, &seed, &dim, &num_points);
        
        #else
            Utility::specify_problem(&seed, &dim, &num_points);
        
        #endif
    }

    MPI_Bcast(&seed, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&dim, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&num_points, 1, MPI_INT, 0, MPI_COMM_WORLD);

    // local array size on individual processes
    int num_points_loc = num_points/size;

    // last points are query
    float* x = Utility::generate_problem(seed, dim, num_points + num_queries);
    Point** points = (Point**)calloc(num_points_loc, sizeof(Point*));

    // counter for local entries of "points" array
    int div_counter = 0;

    // distribute points among threads
    for(int n = 0; n < num_points; ++n){
        int d = floor(n / num_points_loc);
        if(rank == d){
            points[div_counter] = new Point(dim, n + 1, x + n * dim);
            div_counter++;
        }
    }

    // build tree
    Node* tree = build_tree(points, num_points_loc);

    // create array of distances in order to print data 
    float distances[num_queries] = {0.0};
    float min_distances[num_queries] = {0.0};
    
    // for each query, find nearest neighbor
    #pragma omp parallel for schedule(dynamic, 1)
    for(int q = 0; q < num_queries; ++q){
        float* x_query = x + (num_points + q) * dim;
        Point query(dim, num_points_loc + q, x_query);

        Node* res = nearest_neighbor(tree, &query);
        
        // output min-distance (i.e. to query point)
        distances[q] = query.distance(*res->point);

        #if DEBUG
            // in case you want to have further debug information about
            // the query point and the nearest neighbor
            // std::cout << "Query: " << query << std::endl;
            // std::cout << "NN: " << *res->point << std::endl << std::endl;
        #endif
    }
    
    #if DEBUG
        // for measuring your local runtime
        auto tock = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsed_time = tock - tick;
        std::cout << "elapsed time " << elapsed_time.count() << " second" << std::endl;
    #endif

   // reduce minimum distance into process with 0 rank
    MPI_Reduce(distances, min_distances, num_queries, MPI_FLOAT, MPI_MIN, 0, MPI_COMM_WORLD);

    // print results with rank 0
    if(rank == 0) {
        for(int print = 0; print < num_queries; ++print){
            Utility::print_result_line((num_points + print), min_distances[print]);
        }

        std::cout << "DONE" << std::endl;
    }

    // clean-up
    Utility::free_tree(tree);

    for(int n = 0; n < num_points_loc; ++n){
        delete points[n];
    }

    free(points);
    free(x);

    (void)argc;
    (void)argv;

    MPI_Finalize();
    return 0;
}
/***************************************************************************************/
