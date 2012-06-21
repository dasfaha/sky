; ModuleID = 'foo'

define i64 @main() {
entry:
  %0 = alloca double
  %1 = alloca i64
  store i64 200, i64* %1
  %2 = load i64* %1
  ret i64 %2
}
