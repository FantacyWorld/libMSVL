FILE(REMOVE_RECURSE
  "CMakeFiles/mytest.dir/mytest.c.o"
  "mytest.pdb"
  "mytest"
)

# Per-language clean rules from dependency scanning.
FOREACH(lang C)
  INCLUDE(CMakeFiles/mytest.dir/cmake_clean_${lang}.cmake OPTIONAL)
ENDFOREACH(lang)
