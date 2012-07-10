//Self-includes
\#include "${primary_header_include}"

#if len($integrator.integrand.include_dependencies) > 0
//Depedency includes
#for $include_dependency in $integrator.integrand.include_dependencies
\#include "$include_dependency"
#end for
#end if

${integrator.name}::${integrator.name}()
{
    
}

${integrator.name}::~${integrator.name}()
{
    
}

$integrator.evaluation_function.return_type ${integrator.name}::operator()($integrator.evaluation_function.argument_signature)
{
    return 1;
}
