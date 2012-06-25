; ModuleID = 'foo'

%Foo = type { i64, i64 }

define %Foo @main() {
entry:
  %x = alloca %Foo
  %0 = getelementptr inbounds %Foo* %x, i32 0, i32 1
  store i64 20, i64* %0
  %1 = load %Foo* %x
  ret %Foo %1
}
