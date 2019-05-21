#!/bin/sh

repoName=${APPVEYOR_PROJECT_NAME}

mkdir -p @${repoName}/addons
mkdir -p @${repoName}/intercept

cp src/${repoName}_x64.so @${repoName}/intercept/${repoName}_x64.so
cp addons/${repoName}.pbo @${repoName}/addons/${repoName}.pbo

tagToAdd=""
if [ ! -z "$APPVEYOR_REPO_TAG_NAME" ]
then
    echo "Using Tag: $APPVEYOR_REPO_TAG_NAME"
    tagToAdd="$APPVEYOR_REPO_TAG_NAME"
else
    tagToAdd="$APPVEYOR_REPO_COMMIT"
    echo "Tag not found, using commit hash"
fi
zip -r "@${repoName}_$tagToAdd.zip" @${repoName}
appveyor PushArtifact "@${repoName}_$tagToAdd.zip" -DeploymentName zip
echo "Done"