#!/bin/sh

repoName=${APPVEYOR_PROJECT_NAME}

mkdir -p @${repoName}/addons
mkdir -p @${repoName}/intercept

cp src/${repoName}_x64.so @${repoName}/intercept/${repoName}_x64.so
cp addons/${repoName}.pbo @${repoName}/addons/${repoName}.pbo

zip -r @${repoName}.zip @${repoName}

echo "Done"