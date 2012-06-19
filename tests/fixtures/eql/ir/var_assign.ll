; ModuleID = 'foo'

define i64 @main() {
entry:
  %foo = alloca i64
  store i64 20, i64* %foo
  %foo1 = load i64* %foo
  ret i64 %foo1
}
