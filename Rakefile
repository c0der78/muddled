require 'rubygems' if RUBY_VERSION < '1.9'
require 'rake'
require 'rake/builder'
require 'rake/clean'

# create runtime directories
directory 'var'
directory 'var/run'
directory 'var/log'


CLEAN.include("*~", "*.bak")

# call initialize tasks
task :init => ['var', 'var/run', 'var/log']

task :doc do
    sh 'headerdoc2html -o docs src/muddyengine/*.h'

    sh 'gatherheaderdoc docs index.html'
end

task :indent do
    FileList['src/*/*.c', 'src/*/*.h'].each do |src|
      sh "indent -linux #{src}"
    end
end

task :test => [:test_muddyengine, :test_muddyplains] do
    sh "./test_muddyengine"
    sh "./test_muddyplains"
end

Rake::Builder.new do |builder|
  builder.target               = 'muddyplainsd'
  builder.target_type		       = :executable
  builder.architecture         = 'x86_64'
  builder.target_prerequisites = ['libmud.a']
  builder.programming_language = 'c'
  builder.objects_path 		     = 'debug'
  builder.source_search_paths  = [ 'src/muddyplains', 'src/main.c' ]
  builder.include_paths		     = [ 'src', 'src/muddyplains' ]
  builder.library_dependencies = [ 'mud', 'sqlite3', 'lua' ]
  builder.library_paths        = [ '.' ]
  builder.compilation_options  = ['-std=gnu99','-ggdb3','-Wall','-Werror','-Wno-uninitialized','-O0','-DDEBUG',"-DROOT_DIR=\"#{builder.rakefile_path}\""]
end

Rake::Builder.new do |builder|
	builder.target               = 'libmud.a'
  builder.target_type          = :static_library
  builder.target_prerequisites = [:init]
  builder.architecture         = 'x86_64'
  builder.programming_language = 'c'
  builder.objects_path         = 'debug/lib'
  builder.source_search_paths  = [ 'src/muddyengine' ]
  builder.include_paths        = [ 'src' ]
  builder.library_dependencies = [ 'sqlite3', 'lua' ]
  builder.compilation_options  = ['-std=gnu99','-ggdb3','-Wall','-Werror','-Wno-uninitialized','-O0','-DDEBUG']

end

Rake::Builder.new do |builder|
  builder.target               = 'muddyplains'
  builder.target_type		       = :executable
  builder.architecture         = 'x86_64'
  builder.target_prerequisites = ['libmud.a']
  builder.programming_language = 'c'
  builder.objects_path 		     = 'release'
  builder.source_search_paths  = [ 'src/muddyplains', 'src/main.c' ]
  builder.include_paths		     = [ 'src', 'src/muddyplains' ]
  builder.library_dependencies = [ 'mud', 'sqlite3', 'lua' ]
  builder.library_paths        = [ '.' ]
  builder.compilation_options  = ['-std=gnu99','-Wall','-Werror','-Wno-uninitialized','-O3',"-DROOT_DIR=\"#{builder.rakefile_path}\""]
end

Rake::Builder.new do |builder|
  builder.programming_language = 'c'
  builder.target_type          = :executable
  builder.architecture         = 'x86_64'
  builder.target_prerequisites = [ 'libmud.a' ]
  builder.task_namespace       = 'test'
  builder.target               = 'test_muddyplains'
  builder.source_search_paths  = [ 'tests/muddyplains', 'src/muddyplains' ]
  builder.objects_path         = 'tests'
  builder.library_dependencies = [ 'check', 'mud', 'sqlite3', 'lua' ]
  builder.library_paths        = [ '.' ]
  builder.compilation_options  = ['-std=gnu99','-ggdb3','-Wall','-I src','-Werror','-Wno-uninitialized','-O0','-DDEBUG',"-DROOT_DIR=\"#{builder.rakefile_path}\""]
end

Rake::Builder.new do |builder|
  builder.programming_language = 'c'
  builder.target_type          = :executable
  builder.architecture         = 'x86_64'
  builder.target_prerequisites = [ 'libmud.a' ]
  builder.task_namespace       = 'test'
  builder.target               = 'test_muddyengine'
  builder.source_search_paths  = [ 'tests/muddyengine' ]
  builder.objects_path         = 'tests'
  builder.library_dependencies = [ 'check', 'mud', 'sqlite3', 'lua' ]
  builder.library_paths        = [ '.' ]
  builder.compilation_options  = ['-std=gnu99','-ggdb3','-Wall','-I src','-Werror','-Wno-uninitialized','-O0','-DDEBUG',"-DROOT_DIR=\"#{builder.rakefile_path}\""]
end
