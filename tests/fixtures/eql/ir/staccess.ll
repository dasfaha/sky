; ModuleID = 'foo'

%Foo = type { i64, i64 }

define %Foo @main() {
entry:
  %0 = alloca %Foo
  %1 = getelementptr inbounds %Foo* %0, i32 0, i32 1
  store i64 20, i64* %1
  %2 = load %Foo* %0
  ret %Foo %2
}
