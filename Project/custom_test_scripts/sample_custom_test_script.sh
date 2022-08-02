#!/bin/bash

# TODO@Students: Adjust these parameters to suit your needs
GROUP="group-301"
REPOSITORY="hybrid"

echo "Now setting up your repository..."
PRIVATE_KEY_FILE="$(pwd)/.ssh/id_rsa"

# This is your repository access key. Don't share this or your repository will be accessible from the outside.
mkdir -p "$(pwd)/.ssh"
cat <<- EOF > "$PRIVATE_KEY_FILE"
-----BEGIN OPENSSH PRIVATE KEY-----
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
-----END OPENSSH PRIVATE KEY-----

EOF

chmod 600 "$PRIVATE_KEY_FILE"

export GIT_SSH_COMMAND="ssh -i '$PRIVATE_KEY_FILE' -o UserKnownHostsFile=/dev/null -o StrictHostKeyChecking=no"

echo "Cloning your repository..."
git clone "git@gitlab.lrz.de:lrr-tum/teaching/parprog/ss2021/project-material/${GROUP}/${REPOSITORY}.git" || exit 255

# Link the data directory. This is only needed for project 1.
if [ -e "${REPOSITORY}/data/" ]
then
  ln -s "$(pwd)/data/COAUTHORCS" "${REPOSITORY}/data/COAUTHORCS" || exit 255
fi

pushd "${REPOSITORY}" || exit 255

git config user.name "ParProg Submission Server"
git config user.email "noreply@parprog.caps.in.tum.de"

echo -e "\nNow running your tasks."
# You now have full access to your repository
# If you create large amounts of data, please remember to remove it to free up storage space from the submission server.

echo "The current directory is $(pwd). Directory listing:"
ls -la

# TODO@Students: Put your run commands here. Adjust these to make them work for your project.
make hybrid
#perf stat ./hybrid
bash -c 'printf "%d" $RANDOM' | perf record -g mpirun -np 4 --oversubscribe ./hybrid -n 1024 -t 180 COAUTHORCS 1 > test_output.txt
perf report -i perf.data

# TODO@Students If you need to, you can commit and push your changes/measurements by uncommenting the following lines:
#git add .
#git commit -m "ParProg Server Test"
#git push

popd || exit 255


