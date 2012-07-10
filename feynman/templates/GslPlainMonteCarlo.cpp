        gsl_monte_plain_state *s = gsl_monte_plain_alloc($integrator.n_dimensions);
        gsl_monte_plain_integrate(&G, 
                                  lower_bounds, 
                                  upper_bounds, 
                                  $integrator.n_dimensions, 
                                  n_calls, 
                                  _random_number_generator, 
                                  s, 
                                  &result, 
                                  &_error);
        gsl_monte_plain_free(s);
