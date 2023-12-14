<script>
	import { onMount } from "svelte";
	import { fade } from 'svelte/transition';
	import axios from 'axios';
	import CaliforniaCondor from '$lib/California_Condor.jpg'

	$: birds=[]


	onMount(async () => {
	  const fetchData = async () => {
		try {
			const response = await axios.post('http://169.254.157.209:8080/birds/get', {})
		  if (response?.data?.statusCode==="000") {
			const data = response.data.data;
			if(birds.length==0){
				birds = data
			}
			else{
			// Add only the new birds that are not already in the array
            const newBirds = await data.filter(newBird => !birds.some(existingBird => existingBird._id === newBird._id));
            birds = [...birds, ...newBirds];
			console.log("NEW BIRDS", newBirds);
			// birds = birds
			}
		  } else {
			console.error('Failed to fetch data:', response.statusText);
		  }
		} catch (error) {
		  console.error('Error fetching data:', error.message);
		}
	  };
  
	  // Fetch data initially
	  fetchData();
  
	  // Set up an interval to fetch data every 3 seconds
	  const interval = setInterval(fetchData, 3000);
  
	  // Cleanup the interval when the component is unmounted
	  return () => clearInterval(interval);
	});
</script>
<div class="container mx-auto flex justify-center">
	<div class="space-y-10">
		<h1 class="mt-3 h1 text-center">Bird Watcher</h1>
		<div class="flex flex-col-reverse">
			{#each birds as bird}
			<div class="mt-10 card p-4" transition:fade={{  duration: 300 }}>

				<h3 class='text-center mb-3'>{bird.name}</h3>
				<img src={CaliforniaCondor} alt="bird" style="width: 300px;">
				<h5 class='mt-3 text-center'> Detected at:</h5>
				<h5 class='text-center'> {bird.long},{bird.lat}</h5>
			</div>
			{/each}
		</div>
	</div>
</div>

<style>
	.container{
		min-height: 100vh;
	}
</style>