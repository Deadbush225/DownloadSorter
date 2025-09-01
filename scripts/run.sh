#!/usr/bin/env zsh

git add -A
git commit -m "updated build-release.yml for windows"
git tag -f v1.5.2
git push origin main --tags -f