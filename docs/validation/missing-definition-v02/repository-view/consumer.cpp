struct ExternalObject { int value; };
extern ExternalObject* external_lookup();
int review_external() {
    ExternalObject* p = external_lookup();
    return p->value;
}
