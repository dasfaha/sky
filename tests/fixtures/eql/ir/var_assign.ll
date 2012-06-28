; ModuleID = 'foo'

define i64 @main() {
  %foo = alloca i64
  store i64 20, i64* %foo
  %1 = load i64* %foo
  ret i64 %1
}
