; ModuleID = 'foo'

%Foo = type { i64, i64 }

define %Foo @main() {
entry:
  %x = alloca %Foo
  %gep = getelementptr inbounds %Foo* %x, i32 0, i32 1
  store i64 20, i64* %gep
  %x1 = load %Foo* %x
  ret %Foo %x1
}
