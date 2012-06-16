; ModuleID = 'foo'

%Foo = type { i64, double, double }

define void @main() {
entry:
  %myVar = alloca %Foo
  ret void
}
