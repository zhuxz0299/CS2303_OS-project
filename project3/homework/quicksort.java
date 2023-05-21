import java.util.concurrent.ForkJoinPool;
import java.util.concurrent.RecursiveAction;

public class quicksort {
    static class QuickSortTask extends RecursiveAction {
        private final int[] array;
        private final int low;
        private final int high;

        public QuickSortTask(int[] array, int low, int high) {
            this.array = array;
            this.low = low;
            this.high = high;
        }

        @Override
        protected void compute() {
            if (low < high) {
                int pivotIndex = partition(array, low, high);
                QuickSortTask leftTask = new QuickSortTask(array, low, pivotIndex - 1);
                QuickSortTask rightTask = new QuickSortTask(array, pivotIndex + 1, high);
                invokeAll(leftTask, rightTask);
            }
        }

        private int partition(int[] array, int low, int high) {
            int pivot = array[high];
            int i = low - 1;
            for (int j = low; j < high; j++) {
                if (array[j] <= pivot) {
                    i++;
                    swap(array, i, j);
                }
            }
            swap(array, i + 1, high);
            return i + 1;
        }

        private void swap(int[] array, int i, int j) {
            int temp = array[i];
            array[i] = array[j];
            array[j] = temp;
        }
    }

    public static void main(String[] args) {
        int[] array = {5, 9, 1, 3, 2, 8, 4, 7, 6};

        // Using the ForkJoinPool
        ForkJoinPool forkJoinPool = ForkJoinPool.commonPool();

        // Quicksort
        QuickSortTask quickSortTask = new QuickSortTask(array, 0, array.length - 1);
        forkJoinPool.invoke(quickSortTask);

        // Print sorted array
        System.out.println("Sorted array:");
        for (int num : array) {
            System.out.print(num + " ");
        }
        System.out.println();
    }
}