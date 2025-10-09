import grid_shuffler

class Backend:
    def __init__(self):
        pass

    @staticmethod
    def allocate_seats(seating_chart: list[list[str]]) -> list[list[str]]:
        gs = grid_shuffler.GridShuffler(seating_chart)
        if not gs.shuffle() or not gs.validate_result():
            raise RuntimeError("Unable to allocate seats with the given constraints.")
        return gs.get_shuffled_grid()

    @staticmethod
    def read_csv(file_path: str) -> list[list[str]]:
        import pandas as pd
        try:
            df = pd.read_csv(file_path, header=None, dtype=str, engine='c')
            return df.fillna('').values.tolist()
        except Exception as e:
            raise RuntimeError(f"Error reading CSV file: {e}")
