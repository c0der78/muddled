require 'rake/clean'

# constants

COMPILER = "clang"
ARCHIVER = "ar rs"
BIN = "bin"
FLAGS = "-Wall -Werror -std=c99 -Wno-uninitialized -Isrc -DROOT_DIR=\"#{File.dirname(__FILE__)}\""
LIB_NAME = "muddy"
LIB_FLAGS = "-lsqlite3 -llua"
LIB_FLAGS_TEST = "-lcheck"

# binaries 

EXE_RELEASE = "muddyplains"
EXE_DEBUG = "muddyplainsd"
EXE_LIB_TEST = "muddyengine_test"
EXE_TEST = "muddyplains_test"
LIB_RELEASE = "lib#{LIB_NAME}.a"
LIB_DEBUG = "lib#{LIB_NAME}d.a"

# output files

OBJ_LIB_RELEASE = FileList['src/muddyengine/*.c'].gsub("src/muddyengine", 'var/build/release').ext('.o')
OBJ_RELEASE = FileList['src/muddyplains/*.c', 'src/main.c'].gsub('src/muddyplains', 'var/build/release').gsub('src', 'var/build/release').ext('.o')
OBJ_LIB_DEBUG = FileList['src/muddyengine/*.c'].gsub('src/muddyengine', 'var/build/debug').ext('.o')
OBJ_DEBUG = FileList['src/muddyplains/*.c', 'src/main.c'].gsub('src/muddyplains', 'var/build/debug').gsub('src', 'var/build/debug').ext('.o')
OBJ_LIB_TEST = FileList['tests/muddyengine/*.c'].gsub('tests/muddyengine', 'var/build/tests').ext('.o')
OBJ_TEST = FileList['tests/muddyplains/*.c', 'src/muddyplains/*.c'].gsub('tests/muddyplains', 'var/build/tests').gsub('src/muddyplains', 'var/build/tests').ext('.o')
# distribution

DIST_HEADERS = FileList['src/muddyengine/*.h']

DIST_LIBRARY = FileList["#{LIB_RELEASE}"]

DIST_LIB_LOCATION = "/usr/local/lib"

DIST_H_LOCATION = "/usr/local/include/muddyengine"


# files

file "#{LIB_RELEASE}" => OBJ_LIB_RELEASE
file "#{LIB_DEBUG}" => OBJ_LIB_DEBUG
file "#{EXE_RELEASE}" => OBJ_RELEASE
file "#{EXE_DEBUG}" => OBJ_DEBUG
file "#{EXE_LIB_TEST}" => OBJ_LIB_TEST
file "#{EXE_TEST}" => OBJ_TEST

# directories

directory "#{BIN}"
directory 'var/run'
directory 'var/log'
directory 'var/build/debug'
directory 'var/build/release'
directory 'var/build/tests'

CLEAN.include('src/*.orig', 'src/engine/*.orig', 'tests/*.orig', 'var/build')

# tasks

# call initialize tasks
task :init => ["#{BIN}", 'var/build', 'var/run', 'var/log', 'var/build/debug', 'var/build/release', 'var/build/tests']

desc "Format source code"
task :format do
    FileList['src/*/*.c', 'src/*/*.h', 'src/*.c', 'src/*.h', 'tests/*.c', 'tests/*.h'].each do |src|
      sh "astyle #{src}"
    end
end

desc "Compiling release version"
task :release => [:init, "#{LIB_RELEASE}", "#{EXE_RELEASE}"] do
  sh "#{ARCHIVER} #{LIB_RELEASE} #{OBJ_LIB_RELEASE.join(" ")}"
  sh "#{COMPILER} #{FLAGS} -L#{BIN} -l#{LIB_NAME} #{LIB_FLAGS} #{OBJ_RELEASE.join(" ")} -o #{BIN}/#{EXE_RELEASE}"
end

desc "Compiling debug version"
task :debug => [:init, "#{LIB_DEBUG}", "#{EXE_DEBUG}"] do
  sh "#{ARCHIVER} #{LIB_DEBUG} #{OBJ_LIB_DEBUG.join(" ")}"
  sh "#{COMPILER} #{FLAGS} -L#{BIN} -l#{LIB_NAME}d #{LIB_FLAGS} #{OBJ_DEBUG.join(" ")} -o #{BIN}/#{EXE_DEBUG}"
end

desc "Running unit tests"
task :test => [:init, "#{LIB_DEBUG}", "#{EXE_LIB_TEST}", "#{EXE_TEST}"] do 
  sh "#{ARCHIVER} #{BIN}/#{LIB_DEBUG} #{OBJ_LIB_DEBUG.join(" ")}"
  sh "#{COMPILER} #{FLAGS} -L#{BIN} -l#{LIB_NAME}d #{LIB_FLAGS_TEST} #{LIB_FLAGS} #{OBJ_LIB_TEST.join(" ")} -o #{BIN}/#{EXE_LIB_TEST}"
  sh "#{COMPILER} #{FLAGS} -L#{BIN} -l#{LIB_NAME}d #{LIB_FLAGS_TEST} #{LIB_FLAGS} #{OBJ_TEST.join(" ")} -o #{BIN}/#{EXE_TEST}"
  sh "#{BIN}/#{EXE_LIB_TEST}"
  sh "#{BIN}/#{EXE_TEST}"
end

desc "Generating docs"
task :docs do
  sh "headerdoc2html -j -o doc #{LIB_HEADERS}"
  sh "gatherheaderdoc doc index.html"
end

desc "Compiling debug version and running tests"
task :default => [:test] 

# rules

rule '.o' => '%{var/build/debug,src/muddyengine}X.c' do |target|
  sh "#{COMPILER} #{FLAGS} -O0 -g -DDEBUG -c -o #{target.name} #{target.source}"
end
rule '.o' => '%{var/build/debug,src/muddyplains}X.c' do |target|
  sh "#{COMPILER} #{FLAGS} -O0 -g -DDEBUG -c -o #{target.name} #{target.source}"
end

rule '.o' => '%{var/build/debug,src}X.cpp' do |target|
  sh "#{COMPILER} #{FLAGS} -O0 -g -DDEBUG -c -o #{target.name} #{target.source}"
end

rule '.o' => '%{var/build/release,src/muddyengine}X.c' do |target| 
  sh "#{COMPILER} #{FLAGS} -O3 -c -o #{target.name} #{target.source}"
end

rule '.o' => '%{var/build/release,src/muddyplains}X.c' do |target| 
  sh "#{COMPILER} #{FLAGS} -O3 -c -o #{target.name} #{target.source}"
end

rule '.o' => '%{var/build/release,src}X.cpp' do |target| 
  sh "#{COMPILER} #{FLAGS} -O3 -c -o #{target.name} #{target.source}"
end

rule '.o' => '%{var/build/tests,src/muddyplains}X.c' do |target|
  sh "#{COMPILER} #{FLAGS} -c -g -o #{target.name} #{target.source}"
end

rule '.o' => '%{var/build/tests,tests/muddyengine}X.c' do |target|
  sh "#{COMPILER} #{FLAGS} -c -g -o #{target.name} #{target.source}"
end

rule '.o' => '%{var/build/tests,tests/muddyplains}X.c' do |target|
  sh "#{COMPILER} #{FLAGS} -c -g -o #{target.name} #{target.source}"
end


