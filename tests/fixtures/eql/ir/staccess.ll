; ModuleID = 'foo'

%Foo = type { i64, i64 }

define i64 @main() {
  %x = alloca %Foo
  %1 = getelementptr inbounds %Foo* %x, i32 0, i32 1
  store i64 20, i64* %1
  %2 = getelementptr inbounds %Foo* %x, i32 0, i32 1
  %3 = load i64* %2
  ret i64 %3
}
