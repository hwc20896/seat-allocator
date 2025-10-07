import unittest

class TestGridShuffler(unittest.TestCase):
    def test_import_import(self):
        try:
            import backend
        except ImportError as e:
            self.fail(f"Importing backend::grid_shuffler failed with error: {e}")
    def test_read_csv(self):
        import backend
        seating_chart = backend.Backend.read_csv('test.csv')
        for row in seating_chart:
            print(row)
            self.assertIsInstance(row, list)
            for seat in row:
                self.assertIsInstance(seat, str)
        self.assertIsInstance(seating_chart, list)
        self.assertTrue(all(isinstance(row, list) for row in seating_chart))