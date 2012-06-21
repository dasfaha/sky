; ModuleID = 'foo'

%Foo = type { i64 }

define i64 @Foo___foo(%Foo %this) {
entry:
  %0 = alloca %Foo
  store %Foo %this, %Foo* %0
  ret i64 20
}

define i64 @Foo___bar(%Foo %this) {
entry:
  %0 = alloca %Foo
  store %Foo %this, %Foo* %0
  %1 = load %Foo* %0
  %2 = call i64 @Foo___foo(%Foo %1)
  ret i64 %2
}

define %Foo @main() {
entry:
  %0 = alloca %Foo
  %1 = load %Foo* %0
  ret %Foo %1
}
