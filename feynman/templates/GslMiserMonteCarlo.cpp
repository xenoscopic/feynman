        gsl_monte_miser_state *s = gsl_monte_miser_alloc($integrator.n_dimensions);
        gsl_monte_miser_integrate(&G, 
                                  lower_bounds, 
                                  upper_bounds, 
                                  $integrator.n_dimensions, 
                                  n_calls, 
                                  _random_number_generator, 
                                  s, 
                                  &result, 
                                  &_error);
        gsl_monte_miser_free(s);
