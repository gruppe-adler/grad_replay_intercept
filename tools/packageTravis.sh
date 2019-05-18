#!/bin/sh

repoName=${TRAVIS_REPO_SLUG#*/}

mkdir -p @${repoName}/addons
mkdir -p @${repoName}/intercept

cp src/${repoName}.so @${repoName}/intercept/${repoName}.so
cp addons/${repoName}.pbo @${repoName}/addons/${repoName}.pbo

zip -r @${repoName}.zip @${repoName}

echo "Done"