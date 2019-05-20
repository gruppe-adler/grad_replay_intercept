#!/bin/sh

repoName=${APPVEYOR_PROJECT_NAME}

mkdir -p @${repoName}/addons
mkdir -p @${repoName}/intercept

cp src/${repoName}_x64.so @${repoName}/intercept/${repoName}_x64.so
cp addons/${repoName}.pbo @${repoName}/addons/${repoName}.pbo

zip -r @${repoName}.zip @${repoName}

$tagToAdd = ""
if [[ ! -z "$APPVEYOR_REPO_TAG_NAME" ]] {
    Write-Host "Using Tag: $APPVEYOR_REPO_TAG_NAME"
    $tagToAdd = "$APPVEYOR_REPO_TAG_NAME"
} else {
    $tagToAdd = "$APPVEYOR_REPO_COMMIT"
    Write-Host "Tag not found, using commit hash"
}
appveyor PushArtifact @${repoName}_$tagToAdd.zip" -DeploymentName zip
echo "Done"