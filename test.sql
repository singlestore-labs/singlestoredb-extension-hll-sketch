create temporary table sketch_input (id1 int, id2 int);
insert into sketch_input values
  (1, 2), (2, 4), (3, 6), (4, 8), (5, 10), (6, 12), (7, 14), (8, 16), (9, 18), (10, 20);

create table IF NOT EXISTS sketch_intermediate (sketch1 blob, sketch2 blob);

insert into sketch_intermediate select theta_sketch_create_union(id1), theta_sketch_create_union(id2) from sketch_input;

select
    theta_sketch_estimate(sketch1),
    theta_sketch_estimate(sketch2),
    theta_sketch_estimate(theta_sketch_union(sketch1, sketch2)),
    theta_sketch_estimate(theta_sketch_intersect(sketch1, sketch2)),
    theta_sketch_estimate(theta_sketch_anotb(sketch1, sketch2)),
    theta_sketch_estimate(theta_sketch_anotb(sketch2, sketch1))
from sketch_intermediate;

----
--V2

create table IF NOT EXISTS sketch_input (id1 int, id2 int);
insert into sketch_input values
                             (1, 2), (2, 4), (3, 6), (4, 8), (5, 10), (6, 12), (7, 14), (8, 16), (9, 18), (10, 20);
select theta_sketch_print(theta_sketch_union(theta_sketch_create_union(id1), theta_sketch_create_union(id2))) from sketch_input;
