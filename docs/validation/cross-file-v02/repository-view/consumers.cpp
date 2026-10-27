#include "provider.h"
int review_unsafe(bool available) {
    ReviewObject* p = review_lookup(available);
    return p->value;
}
int review_guarded(bool available) {
    ReviewObject* p = review_lookup(available);
    if (!p) return 0;
    return p->value;
}
int review_wrong_guard(bool available) {
    ReviewObject other{3};
    ReviewObject* p = review_lookup(available);
    ReviewObject* q = &other;
    if (!q) return 0;
    return p->value;
}
int review_reassigned(bool available) {
    ReviewObject* p = review_lookup(available);
    if (!p) return 0;
    p = review_lookup(false);
    return p->value;
}
