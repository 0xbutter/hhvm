<?hh

class A<T> {}
class B<reify T> {}
class C<reify Ta, Tb, reify Tc> {}

function f(B<A<B<int>>> $_) { echo "yep\n"; }
f(new B<reify A>());

function g(A<B<int>> $_) { echo "yep\n"; }
g(new A());

function h(C<A<int>, string, reify int> $_) { echo "yep\n"; }
h(new C<reify A<int>, string, reify int>());

function i(B<C<int, string, int>> $_) { echo "yep\n"; }
i(new B<reify C<int, int, int>>());

function j(C<A<int>, string, B<C<int, string, int>>> $_) { echo "yep\n"; }
j(new C<reify A<int>, string, reify B<C<int, int, int>>>());
