#!/usr/bin/env zsh

git add -A
git commit -m "updated build-release.yml for linux"
git tag -f v1.5.1
git push origin main --tags -f