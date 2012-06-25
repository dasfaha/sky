; ModuleID = 'foo'

define i64 @main() {
entry:
  %bar = alloca double
  %foo = alloca i64
  ret i64 200
}
