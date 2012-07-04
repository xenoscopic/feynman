\#pragma once

#for $include_dependency in $integral.include_dependencies
\#include "$include_dependency"
#end for

$integral.signature;
