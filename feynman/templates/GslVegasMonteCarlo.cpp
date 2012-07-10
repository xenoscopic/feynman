        //Do an initial warm-up
        gsl_monte_vegas_state *s = gsl_monte_vegas_alloc($integrator.n_dimensions);
        gsl_monte_vegas_integrate(&G, 
                                  lower_bounds, 
                                  upper_bounds, 
                                  $integrator.n_dimensions, 
                                  10000, 
                                  _random_number_generator, 
                                  s,
                                  &result,
                                  &_error);
        //Now actually converge on a result
        do
        {
            gsl_monte_vegas_integrate(&G, 
                                      lower_bounds, 
                                      upper_bounds, 
                                      $integrator.n_dimensions, 
                                      n_calls/5, 
                                      _random_number_generator, 
                                      s,
                                      &result, 
                                      &_error);
        }
        while(fabs(gsl_monte_vegas_chisq(s) - 1.0) > 0.5);
        gsl_monte_vegas_free(s);
