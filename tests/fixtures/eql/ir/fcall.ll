; ModuleID = 'foo'

%Foo = type { i64 }

define i64 @Foo___foo(%Foo %this) {
entry:
  %this1 = alloca %Foo
  store %Foo %this, %Foo* %this1
  ret i64 20
}

define i64 @Foo___bar(%Foo %this) {
entry:
  %this1 = alloca %Foo
  store %Foo %this, %Foo* %this1
  %this2 = load %Foo* %this1
  %calltmp = call i64 @Foo___foo(%Foo %this2)
  ret i64 %calltmp
}

define %Foo @main() {
entry:
  %x = alloca %Foo
  %x1 = load %Foo* %x
  ret %Foo %x1
}
