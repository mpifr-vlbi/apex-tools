function sum_vec = vec_sum(x,amt)

xtmp=conv(x,1/amt*ones(1,amt));

sum_vec = xtmp(amt:amt:end);