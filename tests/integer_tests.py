import unittest

import atomic
import operator

FETCH_OPS = {
    'add': operator.add,
    'sub': operator.sub,
    'and': operator.and_,
    'xor': operator.xor,
    'or': operator.or_,
    'nand': lambda x, y: ~(x & y),
}


class TestAtomicInteger(unittest.TestCase):
    def test_init(self):
        x = atomic.Integer()
        self.assertEqual(x.get(), 0)

        x = atomic.Integer(3)
        self.assertEqual(x.get(), 3)

    def test_set(self):
        x = atomic.Integer()
        x.set(99)
        self.assertEqual(x.get(), 99)

    def test_get_and_set(self):
        x = atomic.Integer(1)
        ret = x.get_and_set(99)
        self.assertEqual(ret, 1)
        self.assertEqual(x.get(), 99)

    def test_compare_and_set(self):
        x = atomic.Integer(1)

        ret = x.compare_and_set(1, 99)
        self.assertTrue(ret)
        self.assertEqual(x.get(), 99)

        ret = x.compare_and_set(1, 100)
        self.assertFalse(ret)
        self.assertEqual(x.get(), 99)

    def test_get_and(self):
        for op, f in FETCH_OPS.items():
            x = atomic.Integer(1)
            ret = getattr(x, 'get_and_%s' % op)(2)
            self.assertEqual(x.get(), f(1, 2))
            self.assertEqual(ret, 1)

    def test_and_get(self):
        for op, f in FETCH_OPS.items():
            x = atomic.Integer(1)
            ret = getattr(x, '%s_and_get' % op)(2)
            self.assertEqual(x.get(), f(1, 2))
            self.assertEqual(ret, f(1, 2))


if __name__ == '__main__':
    unittest.main()
