for N in {1..500}
do
    ruby client.rb a& 
done
wait  # This ensures the script waits for all background processes to finish
