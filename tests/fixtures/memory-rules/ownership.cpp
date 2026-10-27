#include <cstdlib>
#include <memory>

int released_alias() {
    int* owner = new int(7);
    int* alias = owner;
    delete owner;
    return *alias;
}

int retained_owner() {
    auto owner = std::make_shared<int>(7);
    auto retained = owner;
    int* alias = owner.get();
    owner.reset();
    return *alias;
}

int abandoned_allocation(bool stop) {
    int* owner = new int(7);
    if (stop) return 0;
    int result = *owner;
    delete owner;
    return result;
}

int scoped_cleanup(bool stop) {
    auto owner = std::make_unique<int>(7);
    if (stop) return 0;
    return *owner;
}

int replace_after_release() {
    int* owner = new int(7);
    delete owner;
    owner = new int(9);
    int result = *owner;
    delete owner;
    return result;
}

extern void accept_buffer(void*);
void unknown_transfer() {
    void* buffer = std::malloc(16);
    if (!buffer) return;
    accept_buffer(buffer);
}
