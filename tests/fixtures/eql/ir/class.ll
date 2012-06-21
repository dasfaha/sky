; ModuleID = 'foo'

%Foo.0 = type { i64 }

define i64 @Foo___baz(%Foo.0 %this) {
entry:
  %0 = alloca %Foo.0
  store %Foo.0 %this, %Foo.0* %0
  ret i64 20
}

define %Foo.0 @main() {
entry:
  %0 = alloca %Foo.0
  %1 = load %Foo.0* %0
  ret %Foo.0 %1
}
