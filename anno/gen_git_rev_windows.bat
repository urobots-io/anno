echo const char* git_rev = R^"( >  git_rev.h
git rev-parse --short HEAD >> git_rev.h
echo )^";  >>  git_rev.h