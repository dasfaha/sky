; ModuleID = 'foo'

%Foo = type { i64, double, double }

define void @main() {
entry:
  %0 = alloca %Foo
  ret void
}
