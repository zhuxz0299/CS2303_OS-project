import java.util.Arrays;
import java.util.concurrent.ForkJoinPool;
import java.util.concurrent.RecursiveAction;

public class mergesort {
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