; ModuleID = 'foo'

define i64 @main() {
entry:
  %0 = alloca i64
  store i64 20, i64* %0
  %1 = load i64* %0
  ret i64 %1
}
