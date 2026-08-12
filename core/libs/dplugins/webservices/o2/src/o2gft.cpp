#include "o2gft.h"

static const char *GftScope = "https://www.googleapis.com/auth/fusiontables";

O2Gft::O2Gft(QObject *parent): O2Google(parent) {
    setScope(GftScope);
}

#include "moc_o2gft.cpp"
