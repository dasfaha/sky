; ModuleID = 'foo'

%Foo.0 = type { i64 }

define i64 @Foo___baz(%Foo.0* %this) {
  %1 = alloca %Foo.0*
  store %Foo.0* %this, %Foo.0** %1
  %2 = load %Foo.0** %1
  %3 = getelementptr inbounds %Foo.0* %2, i32 0, i32 0
  store i64 20, i64* %3
  %4 = load %Foo.0** %1
  %5 = getelementptr inbounds %Foo.0* %4, i32 0, i32 0
  %6 = load i64* %5
  ret i64 %6
}

define void @main() {
  %y = alloca %Foo.0
  %x = alloca %Foo.0
  %1 = getelementptr inbounds %Foo.0* %x, i32 0, i32 0
  store i64 10, i64* %1
  ret void
}
