"""
Python implementation of the `ulisse_envelope_raw` and `ulisse_envelope_normalized`
functions from the ULISSE library.
"""

from typing import Dict, List, Tuple

import numpy as np


def flip_env_infinities(envelopes: List[Dict[str, np.ndarray]]) -> None:
    """See `src/Summarization/Envelope.cpp:flip_env_infinities`"""
    for envelope in envelopes:
        for i in range(len(envelope["lower"])):
            if envelope["lower"][i] > envelope["upper"][i]:
                envelope["lower"][i] = -np.inf
                envelope["upper"][i] = np.inf


def ulisse_envelope_raw(ts: np.ndarray, env_params: Tuple[int, int, int, int]) -> List[Dict[str, np.ndarray]]:
    """See `src/Summarization/Envelope.cpp:ulisse_envelope_raw`"""
    pos_per_env, segment_len, l_min, l_max = env_params

    segments_per_env = l_max // segment_len
    num_env = (len(ts) - l_min + pos_per_env) // pos_per_env
    envelopes = [
        {
            "lower": np.full(segments_per_env, np.inf),
            "upper": np.full(segments_per_env, -np.inf),
        }
        for _ in range(num_env)
    ]

    paa_acc = 0.0

    for last_ind, data_point in enumerate(ts):
        paa_acc += data_point
        subs_len = last_ind + 1
        if subs_len > segment_len:
            paa_acc -= ts[last_ind - segment_len]

        segments_in_subs = min(l_max, subs_len) // segment_len

        paa_val = paa_acc / segment_len
        for seg_ind in range(segments_in_subs):
            first_ind = last_ind + 1 - (seg_ind + 1) * segment_len
            if len(ts) - first_ind >= l_min:
                envelope = envelopes[first_ind // pos_per_env]
                envelope["lower"][seg_ind] = min(envelope["lower"][seg_ind], paa_val)
                envelope["upper"][seg_ind] = max(envelope["upper"][seg_ind], paa_val)

    flip_env_infinities(envelopes)
    return envelopes


def calculate_mu_and_sigma(sum_val: float, sq_sum_val: float, length: int) -> Tuple[float, float]:
    """See `src/Util/utilities.cpp:calculate_mu_and_sigma`"""
    mu = sum_val / length
    sigma = np.sqrt((sq_sum_val / length) - (mu**2))
    return mu, sigma


def ulisse_envelope_normalized(ts: np.ndarray, env_params: Tuple[int, int, int, int]) -> List[Dict[str, np.ndarray]]:
    """See `src/Summarization/Envelope.cpp:ulisse_envelope_normalized`"""
    pos_per_env, segment_len, l_min, l_max = env_params

    segments_per_env = l_max // segment_len
    num_env = (len(ts) - l_min + pos_per_env) // pos_per_env
    envelopes = [
        {
            "lower": np.full(segments_per_env, np.inf),
            "upper": np.full(segments_per_env, -np.inf),
        }
        for _ in range(num_env)
    ]

    sum_accs = np.zeros(len(ts) + 1)
    sq_sum_accs = np.zeros(len(ts) + 1)

    for last_ind, data_point in enumerate(ts):
        sum_accs[last_ind + 1] = sum_accs[last_ind] + data_point
        sq_sum_accs[last_ind + 1] = sq_sum_accs[last_ind] + data_point * data_point

        start_min = max(0, last_ind + 1 - l_max)
        start_max = last_ind + 1 - l_min

        for start in range(start_min, start_max + 1):
            subs_len = last_ind - start + 1
            mu, sigma = calculate_mu_and_sigma(
                sum_accs[last_ind + 1] - sum_accs[start],
                sq_sum_accs[last_ind + 1] - sq_sum_accs[start],
                subs_len,
            )

            num_seg_in_subs = subs_len // segment_len
            for seg_ind in range(num_seg_in_subs):
                paa_val = (
                    sum_accs[start + (seg_ind + 1) * segment_len] - sum_accs[start + seg_ind * segment_len]
                ) / segment_len
                paa_val = (paa_val - mu) / sigma

                envelope = envelopes[start // pos_per_env]
                envelope["lower"][seg_ind] = min(envelope["lower"][seg_ind], paa_val)
                envelope["upper"][seg_ind] = max(envelope["upper"][seg_ind], paa_val)

    flip_env_infinities(envelopes)
    return envelopes
