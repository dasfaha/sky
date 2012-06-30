; ModuleID = 'foo'

%Foo = type { i64 }
%Bar = type { i64 }

define void @Foo___next(%Foo* %this, %Bar* %value) {
  %1 = alloca %Bar*
  %2 = alloca %Foo*
  store %Foo* %this, %Foo** %2
  store %Bar* %value, %Bar** %1
  %3 = load %Foo** %2
  %4 = getelementptr inbounds %Foo* %3, i32 0, i32 0
  %5 = load i64* %4
  %6 = add i64 %5, 1
  %7 = load %Foo** %2
  %8 = getelementptr inbounds %Foo* %7, i32 0, i32 0
  store i64 %6, i64* %8
  %9 = load %Foo** %2
  %10 = getelementptr inbounds %Foo* %9, i32 0, i32 0
  %11 = load i64* %10
  %12 = mul i64 %11, 2
  %13 = load %Bar** %1
  %14 = getelementptr inbounds %Bar* %13, i32 0, i32 0
  store i64 %12, i64* %14
  ret void
}

define i1 @Foo___eof(%Foo* %this) {
  %1 = alloca %Foo*
  store %Foo* %this, %Foo** %1
  %2 = load %Foo** %1
  %3 = getelementptr inbounds %Foo* %2, i32 0, i32 0
  %4 = load i64* %3
  %5 = icmp eq i64 %4, 5
  ret i1 %5
}

define i64 @main() {
  %b = alloca %Bar
  %foo = alloca %Foo
  %i = alloca i64
  br label %1

; <label>:1                                       ; preds = %3, %0
  %2 = call i1 @Foo___eof(%Foo* %foo)
  br i1 %2, label %6, label %3

; <label>:3                                       ; preds = %1
  call void @Foo___next(%Foo* %foo, %Bar* %b)
  %4 = getelementptr inbounds %Bar* %b, i32 0, i32 0
  %5 = load i64* %4
  store i64 %5, i64* %i
  br label %1

; <label>:6                                       ; preds = %1
  %7 = load i64* %i
  ret i64 %7
}
