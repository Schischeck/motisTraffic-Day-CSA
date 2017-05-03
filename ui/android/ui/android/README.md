## Generate flatbuffers classes for android

In Project Root:

find protocol -name "*.fbs" -exec ./build/external_lib/flatbuffers/flatc32 --java -I protocol -o ui/android/app/src/main/java {} \;
