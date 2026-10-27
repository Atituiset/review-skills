#include "provider.h"
ReviewObject* review_lookup(bool available) {
    static ReviewObject object{7};
    if (!available) return nullptr;
    return &object;
}
