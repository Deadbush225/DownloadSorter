#!/usr/bin/env zsh

git add -A
git commit -m "enhanced DLL pruning and added deployment analysis"
git tag -f v0.0.8
git push origin main --tags -f