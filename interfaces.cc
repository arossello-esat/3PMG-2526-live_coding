
vtable* ptr = vable_A;
struct A {
    virtual void f() {
        std::print("en A");
    }
    virtual void g() {
        std::print("en A tambien");
    }
    int i;
};

vtable* ptr = vable_B;
struct B : A {
    virtual void f() {
        std::print("en B");
    }
};


struct vtable_A {
    void (*f)(A&this) = A::f;
    void (*g)(A&this) = A::g;
};

struct vtable_B {
    void (*f)(B&this) = B::f;
    void (*g)(A&this) = A::g;
};

void otra(A& a) {
    (&a-4)->f();
}

int main(int,char**) {

    B b;
    otra(b);
    A a;
    otra(a);

}