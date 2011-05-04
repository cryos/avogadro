#!/usr/bin/env bash

# Run this script to set up your clone of Avogadro for development.

# Set up user name and email address
setup_git_user() {
  read -ep "Please enter your full name, e.g. 'John E. Doe': " name
  echo "Name: '$name'"
  git config user.name "$name"
  read -ep "Please enter your email address, e.g. 'john@doe.com': " email
  echo "Email address: '$email'"
  git config user.email "$email"
}

# Infinite loop until confirmation information is correct
for (( ; ; ))
do
  # Display the final user information.
  gitName=$(git config user.name)
  gitEmail=$(git config user.email)
  echo "Your commits will have the following author information:

  $gitName <$gitEmail>
"
  read -ep "Is the name and email address above correct? [Y/n] " correct
  if [ "$correct" == "n" ] || [ "$correct" == "N" ]; then
    setup_git_user
  else
    break
  fi
done

# Set up gerrit remote
gerrit_user() {
  read -ep "Enter your gerrit user (Gerrit Settings/Profile) [$USER]: " gu
  if [ "$gu" == "" ]; then
    gu=$USER
  fi
  echo -e "\nConfiguring 'gerrit' remote with user '$gu'..."
  if git config remote.gerrit.url >/dev/null; then
    # Correct the remote url
    git remote set-url gerrit ssh://$gu@review.source.kitware.com/avogadro || \
      die "Could not set gerrit remote."
  else
    # Add a new one
    git remote add gerrit ssh://$gu@review.source.kitware.com/avogadro || \
      die "Could not add gerrit remote."
  fi
  cat << EOF

For more information on working with Gerrit,

  http://avogadro.openmolecules.net/wiki/Working_with_Gerrit
EOF
}

# Make sure we are inside the repository.
cd "$(echo "$0"|sed 's/[^/]*$//')"

for (( ; ; ))
do
  echo "The configured Gerrit remote URL is:"
  echo
  git config remote.gerrit.url
  gu=`git config remote.gerrit.url | sed -e 's/^ssh:\/\///' | sed -e 's/@review.source.kitware.com\/avogadro//'`
  echo
  read -ep "Is the username and URL correct? [Y/n]: " correct
  if [ "$correct" == "n" ] || [ "$correct" == "N" ]; then
    gerrit_user
  else
    echo "If you have not done so, you should fetch the gerrit hook."
    echo
    echo "scp -p $gu@review.source.kitware.com:hooks/commit-msg .git/hooks/"
    echo
    break
  fi
done
cat << EOF

Setting up some useful git aliases for you. This can be used by typing git and
the alias name. You can inspect all aliases in this script, or by reading
.git/config in your clone.

  prepush          - view a short form of the commits about to be pushed,
                     relative to origin/master
  gerrit-push      - push the current topic branch to Gerrit for code review.

  gerrit-push-1-0  - push the current topic branch for review if intended for
                     1.0 branch

EOF

git config alias.prepush 'log --graph --stat origin/master..'
git_branch="\$(git symbolic-ref HEAD | sed -e 's|^refs/heads/||')"
git config alias.gerrit-push "!sh -c \"git push gerrit HEAD:refs/for/master/${git_branch}\""
git config alias.gerrit-push-1-0 "!sh -c \"git push gerrit HEAD:refs/for/1.0/${git_branch}\""

