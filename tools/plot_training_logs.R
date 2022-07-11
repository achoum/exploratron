library(ggplot2)
theme_set(theme_bw())


setwd("D:/projects/exploratron")

ma <- function(x, n = 100){filter(x, rep(1 / n, n), sides = 2)}

ap = function (plot,name,base="genetic") {
  path = paste0("training_logs/",base,"/",name,".csv")
  print(path)
  d = read.csv(path)
  d$smoothed_max_fitness = ma(d$max_fitness)
  #d$smoothed_median_fitness = ma(d$median_fitness)
  
  plot = plot +
    geom_path(data=d, aes(x=generation,y=smoothed_max_fitness, color=paste0(name," smooth")))
    #geom_path(data=d, aes(x=generation,y=max_fitness, color=name))
    #geom_path(data=d, aes(x=generation,y=smoothed_median_fitness, color=name))
    #geom_path(data=d, aes(x=generation,y=max_fitness, color="max")) +
    #geom_path(data=d, aes(x=generation,y=median_fitness, color="mean")) +
    #geom_smooth(data=d, method = "gam",aes(x=generation,y=max_fitness, color=name), se = FALSE, size=0.8) 
    #geom_smooth(data=d, method = "gam",aes(x=generation,y=median_fitness, color="mean"), se = FALSE)
  return (plot)
}

p = ggplot()
#p = ap(p, "pop-100_elite-5_random-10_mutation-rate-5_mutation-scale-0.1_one-point")
#p = ap(p, "pop-100_elite-5_random-10_mutation-rate-5_mutation-scale-0.05_one-point")
#p = ap(p, "pop-100_elite-5_random-10_mutation-rate-5_mutation-scale-0.2_one-point")
#p = ap(p, "pop-100_elite-5_random-10_mutation-rate-all_mutation-scale-0.2_one-point")
#p = ap(p, "pop-100_elite-5_random-10_mutation-rate-all_mutation-scale-0.5_two-point")
#p = ap(p, "pop-100_elite-5_random-10_mutation-rate-all_mutation-scale-0.5_no-cross")
#p = ap(p, "pop-100_elite-5_random-10_mutation-rate-all_mutation-scale-0.4_one-point_9x9")

#p = ap(p, "pop-100_elite-5_random-10_mutation-rate-all_mutation-scale-0.5_one-point")
#p = ap(p, "pop-100_elit-5_to-ra-0_cross1p-r-0.9_mut-all-0.5")
#p = ap(p, "pop-100_elit-5_to-ra-0_cross1p-r-1_mut-all-0.5")
#p = ap(p, "r2_pop-100_elit-5_to-ra-0_cross1p-r-0.98_mut-sm-0.1x0.3")

p = ap(p, "rng_wall_pop-50_evalr20_h", "evo")
p = ap(p, "rng_wall_pop-50_evalr20_h-20", "evo")



print(p)
