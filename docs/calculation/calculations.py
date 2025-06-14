# %%
import re
from itertools import combinations

import sympy as sp

# Define symbols
s, l, p, k, i, j, ii, jj = sp.symbols("s l p k i j ii jj")

subs = (p, p + l - 1)
seg = (p + (k - 1) * s, p + k * s - 1)


# %%

# & = & \frac{-1}{2}\sum_{j'=p}^{p+l-1}j'^2 + \frac{2p+2l-1}{2}\sum_{j'=p}^{p+l-1}j' - \frac{p^2-p}{2}l \\[8pt]


def get_sum_min(outer_ind, range=subs, ind_name="II"):
    ind = sp.symbols(ind_name)
    start, end = range
    return sp.Sum(ind, (ind, start, outer_ind - 1)) + (end - outer_ind + 1) * outer_ind


def double_sum_min(ranges, ind_name="I"):
    ind = sp.symbols(ind_name)
    return sp.Sum(get_sum_min(ind, ranges[1], f"{ind_name}2"), (ind, ranges[0]))


sum_sum_min = double_sum_min([subs] * 2).doit().simplify()
sum_sum_min

# print("Double sum of minimums:")
# print(sp.latex(sp.expand(sum_sum_min.doit()).simplify()))

# %%

ev_s_sigma_squared = (
    s**2 / l * sp.Sum(i - 2 / l * get_sum_min(i) + 1 / l**2 * sum_sum_min, (i, *subs))
)

print("Expected value of s times sigma squared:")
print(sp.latex(sp.expand(ev_s_sigma_squared.doit()).simplify()))
ev_s_sigma_squared.doit().simplify()

# %%

ev_S_mu_diff_squared_term_1 = double_sum_min([seg] * 2).doit().simplify()

print("Expected value of S times mu difference squared term 1:")
print(sp.latex(sp.expand(ev_S_mu_diff_squared_term_1.doit()).simplify()))
ev_S_mu_diff_squared_term_1.doit().simplify()

# %%

ev_S_mu_diff_squared_term_2 = sp.Sum(get_sum_min(i, subs), (i, *seg))
# ev_S_mu_diff_squared_term_2 = (
#     double_sum_min([seg_start, p], [p * k * s - 1, subs_end]).doit().simplify()
# )

print("Expected value of S times mu difference squared term 2:")
print(sp.latex(sp.expand(ev_S_mu_diff_squared_term_2.doit()).simplify()))
ev_S_mu_diff_squared_term_2.doit().simplify()

# %%

ev_S_mu_diff_squared_term_3 = double_sum_min([subs] * 2).doit().simplify()

print("Expected value of S times mu difference squared term 3:")
print(sp.latex(sp.expand(ev_S_mu_diff_squared_term_3.doit()).simplify()))
ev_S_mu_diff_squared_term_3.doit().simplify()

# %%

ev_S_mu_diff_squared = (
    ev_S_mu_diff_squared_term_1
    - 2 / l * ev_S_mu_diff_squared_term_2
    + 1 / l**2 * ev_S_mu_diff_squared_term_3
)

print("Expected value of S times mu difference squared:")
print(sp.latex(sp.expand(ev_S_mu_diff_squared.doit()).simplify()))

# %%


def get_pairings(sum_inds: list[int], sort: bool = True) -> set[tuple]:
    ind_pairings = set()
    for ind1, ind2 in combinations(sum_inds, 2):
        remaining = sum_inds.copy()
        remaining.remove(ind1)
        remaining.remove(ind2)
        if sort:
            ind_pairings.add(tuple(sorted([(ind1, ind2), tuple(remaining)])))
        else:
            ind_pairings.add(((ind1, ind2), tuple(remaining)))
    return ind_pairings


# %%


def four_sum_min(ranges):
    ind_pairings = get_pairings([0, 1, 2, 3])

    expressions = []
    for (i1, i2), (i3, i4) in ind_pairings:
        iv1, iv3 = sp.symbols("i1 i3")
        expr = sp.Sum(
            get_sum_min(iv1, ranges[i2], "i2")
            * sp.Sum(
                get_sum_min(iv3, ranges[i4], "i4"),
                (iv3, ranges[i3]),
            ),
            (iv1, ranges[i1]),
        )
        # expr = double_sum_min([ranges[i1], ranges[i2]]) * double_sum_min(
        #     [ranges[i3], ranges[i4]]
        # )
        expressions.append(expr)
    return sp.Add(*expressions)


four_sum_min([subs] * 4).doit().simplify()

# %%


def three_sum_one_sq_min(ranges):
    ind_pairings = get_pairings([0, 0, 1, 2])

    expressions = []
    for (i1, i2), (i3, i4) in ind_pairings:
        if i1 == i2:
            iv1, iv3 = sp.symbols("i1 i2")
            expr = sp.Sum(
                iv1 * sp.Sum(get_sum_min(iv3, ranges[i4], "i4"), (iv3, ranges[i3])),
                (iv1, ranges[i1]),
            )
        else:
            inner_2 = i3 if i1 != i3 else i4
            inner_2_name = "i3" if i1 != i3 else "i4"
            iv1 = sp.symbols("i1")
            expr = 2 * sp.Sum(
                get_sum_min(iv1, ranges[i2], "i2")
                * get_sum_min(iv1, ranges[inner_2], inner_2_name),
                (iv1, ranges[0]),
            )
        expressions.append(expr)
    return sp.Add(*expressions)


three_sum_one_sq_min([subs] * 3).doit().simplify()

# %%


def double_sum_two_sq_min(ranges):
    ind_pairings = get_pairings([0, 0, 1, 1])

    expressions = []
    for (i1, i2), (i3, i4) in ind_pairings:
        if i1 == i2:
            iv1, iv3 = sp.symbols("i1 i3")
            expr = sp.Sum(iv1 * sp.Sum(iv3, (iv3, ranges[i3])), (iv1, ranges[i1]))
        else:
            iv1, iv2 = sp.symbols("i1 i2")
            expr = 2 * sp.Sum(
                sp.Sum(iv2**2, (iv2, ranges[i2][0], iv1 - 1))
                + (ranges[i2][1] - iv1 + 1) * iv1**2,
                (iv1, ranges[i1]),
            )
        expressions.append(expr)
    return sp.Add(*expressions)


double_sum_two_sq_min([subs] * 2).doit().simplify()


# %%


def six_sum_min(ranges):
    ind_pairings = get_pairings([0, 1, 2, 3, 4, 5], sort=False)

    expressions = []
    for (i1, i2), remaining in ind_pairings:
        expr = double_sum_min([ranges[i1], ranges[i2]]) * four_sum_min(
            [ranges[i] for i in remaining]
        )
        expressions.append(expr)
    return sp.Add(*expressions)


six_sum_min([subs] * 6).doit().simplify()

# %%

cov_terms = [
    s**2 / l * three_sum_one_sq_min([subs] + [seg] * 2),
    -2 * s**2 / l**2 * four_sum_min([subs] * 2 + [seg] * 2),
    -2 * s**2 / l**2 * three_sum_one_sq_min([subs] * 2 + [seg]),
    +4 * s**2 / l**3 * four_sum_min([subs] * 3 + [seg]),
    +(s**2) / l**3 * three_sum_one_sq_min([subs] * 3),
    -2 * s**2 / l**4 * four_sum_min([subs] * 4),
]

cov = sp.Add(*cov_terms).doit().simplify()
cov

# %%

var_terms = [
    double_sum_two_sq_min([subs] * 2),
    4 / l**2 * four_sum_min([subs] * 4),
    1 / l**4 * six_sum_min([subs] * 6),
    +2 / l**2 * four_sum_min([subs] * 4),
]

var = (s**4 / l**2 * sp.Add(*var_terms) - ev_s_sigma_squared**2).doit().simplify()
var

# %%

VE_taylor_approx = (
    ev_S_mu_diff_squared / ev_s_sigma_squared
    - cov / ev_s_sigma_squared**2
    + var * ev_S_mu_diff_squared / ev_s_sigma_squared**3
)
VE_taylor_approx.doit().simplify()


# %%
# (l*s**2*(4*l**2 + 12*l*p - 6*l + 12*p**2 - 12*p - 3*(l + 2*p - 1)**2 + 2)**2*(6*k**2*s**3 + 12*k*p*s**2 - 6*k*s**3 - 6*k*s**2 + 2*l**2 + 6*l*p + l*s*(6*k*s**2 + 6*p*s - 4*s**2 - 3*s + 1) - 3*l + 6*p**2*s - 6*p*s**2 + 6*p*s*(p - 1) - 6*p*s + 2*s**3 + 3*s**2 - 3*s*(2*l + 2*p - 1)*(2*k*s + 2*p - s - 1) + s + 1) - l*(l**2*(90*k**4*l*s**4 - 180*k**4*s**6 - 180*k**3*l**2*s**3 + 360*k**3*l*s**5 - 360*k**3*l*s**4 + 720*k**3*s**6 + 180*k**2*l**3*s**2 - 300*k**2*l**2*s**4 + 360*k**2*l**2*s**3 - 360*k**2*l*p*s**3 - 720*k**2*l*s**5 + 390*k**2*l*s**4 + 180*k**2*l*s**3 - 90*k**2*l*s**2 + 720*k**2*p*s**5 - 540*k**2*p*s**4 - 1080*k**2*s**6 - 360*k**2*s**5 + 390*k**2*s**4 - 90*k*l**4*s + 120*k*l**3*s**3 - 120*k*l**3*s**2 + 360*k*l**2*p*s**2 + 240*k*l**2*s**4 + 30*k*l**2*s**3 - 180*k*l**2*s**2 + 90*k*l**2*s - 720*k*l*p*s**4 + 1440*k*l*p*s**3 + 240*k*l*s**5 + 300*k*l*s**4 - 840*k*l*s**3 + 120*k*l*s**2 - 1800*k*p*s**5 + 1080*k*p*s**4 + 720*k*s**6 + 900*k*s**5 - 780*k*s**4 + 150*k*s**3 + 18*l**5 - 20*l**4*s**2 - 120*l**3*p*s + 24*l**3*s**2 + 60*l**3*s - 30*l**3 + 240*l**2*p*s**3 - 90*l**2*p*s**2 - 120*l**2*s**4 - 180*l**2*s**3 + 43*l**2*s**2 + 15*l**2*s + 540*l*p**2*s**2 - 120*l*p*s**4 - 180*l*p*s**3 - 870*l*p*s**2 + 150*l*p*s + 80*l*s**5 + 70*l*s**3 + 240*l*s**2 - 75*l*s + 12*l - 1080*p**2*s**4 + 1080*p**2*s**3 - 540*p**2*s**2 + 960*p*s**5 + 540*p*s**4 - 1380*p*s**3 + 720*p*s**2 - 180*s**6 - 480*s**5 + 120*s**4 + 420*s**3 - 320*s**2) + l*s**2*(-120*k**2*s**2 - 60*k*s**3 + 240*k*s**2 - 60*p*s**2 + 240*p*s - 210*p + 40*s**3 - 90*s**2 - 130*s + 141) - 18*s**2)*(4*l**2 + 12*l*p - 6*l + 12*p**2 - 12*p - 3*(l + 2*p - 1)**2 + 2)/5 + s**2*(120*l**6 + 1080*l**5*p - 356*l**5 + 3240*l**4*p**2 - 2208*l**4*p + 474*l**4 + 3240*l**3*p**3 - 3348*l**3*p**2 + 1998*l**3*p - 407*l**3 + 1620*l**2*p**2 - 1140*l**2*p + 255*l**2 + 270*l*p - 101*l + 15)*(6*k**2*s**3 + 12*k*p*s**2 - 6*k*s**3 - 6*k*s**2 + 2*l**2 + 6*l*p + l*s*(6*k*s**2 + 6*p*s - 4*s**2 - 3*s + 1) - 3*l + 6*p**2*s - 6*p*s**2 + 6*p*s*(p - 1) - 6*p*s + 2*s**3 + 3*s**2 - 3*s*(2*l + 2*p - 1)*(2*k*s + 2*p - s - 1) + s + 1)/2)/(l*s**4*(4*l**2 + 12*l*p - 6*l + 12*p**2 - 12*p - 3*(l + 2*p - 1)**2 + 2)**3)

# print VE_taylor_approx.doit().simplify() as a python expression without adding the symbol definitions
expr = VE_taylor_approx.doit().simplify()
sp.ccode(expr, assign_to="paa_var")
