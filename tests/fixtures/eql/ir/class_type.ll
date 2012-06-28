; ModuleID = 'foo'

%Foo = type { i64, double, double }

define void @main() {
  %myVar = alloca %Foo
  ret void
}
