import java.util.Arrays;
import java.util.concurrent.ForkJoinPool;
import java.util.concurrent.RecursiveAction;

public class ForkJoinSortingApp {

    // Quicksort implementation
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

    // Mergesort implementation
    static class MergeSortTask extends RecursiveAction {
        private final int[] array;
        private final int low;
        private final int high;

        public MergeSortTask(int[] array, int low, int high) {
            this.array = array;
            this.low = low;
            this.high = high;
        }

        @Override
        protected void compute() {
            if (low < high) {
                int mid = (low + high) / 2;
                MergeSortTask leftTask = new MergeSortTask(array, low, mid);
                MergeSortTask rightTask = new MergeSortTask(array, mid + 1, high);
                invokeAll(leftTask, rightTask);
                merge(array, low, mid, high);
            }
        }

        private void merge(int[] array, int low, int mid, int high) {
            int[] temp = Arrays.copyOfRange(array, low, mid + 1);
            int i = 0, j = mid + 1, k = low;

            while (i < temp.length && j <= high) {
                if (temp[i] <= array[j]) {
                    array[k++] = temp[i++];
                } else {
                    array[k++] = array[j++];
                }
            }

            while (i < temp.length) {
                array[k++] = temp[i++];
            }
        }
    }

    public static void main(String[] args) {
        int[] array = {5, 9, 1, 3, 2, 8, 4, 7, 6};

        // Using the ForkJoinPool
        ForkJoinPool forkJoinPool = ForkJoinPool.commonPool();

        // Quicksort
        QuickSortTask quickSortTask = new QuickSortTask(array, 0, array.length - 1);
        forkJoinPool.invoke(quickSortTask);

        // Mergesort
        MergeSortTask mergeSortTask = new MergeSortTask(array, 0, array.length - 1);
        forkJoinPool.invoke(mergeSortTask);

        // Print sorted array
        System.out.println("Sorted array:");
        for (int num : array) {
            System.out.print(num + " ");
        }
        System.out.println();
    }
}