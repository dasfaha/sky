; ModuleID = 'foo'

%Foo = type { i64 }

define i64 @Foo___foo(%Foo* %this) {
  %1 = alloca %Foo*
  store %Foo* %this, %Foo** %1
  ret i64 20
}

define i64 @Foo___bar(%Foo* %this) {
  %1 = alloca %Foo*
  store %Foo* %this, %Foo** %1
  %2 = load %Foo** %1
  %3 = call i64 @Foo___foo(%Foo* %2)
  ret i64 %3
}

define %Foo @main() {
  %x = alloca %Foo
  %1 = load %Foo* %x
  ret %Foo %1
}
