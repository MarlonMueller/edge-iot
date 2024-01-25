<script>
	import { onMount } from 'svelte';
	import { fade } from 'svelte/transition';
	import { SlideToggle } from '@skeletonlabs/skeleton';
	import axios from 'axios';
	import CaliforniaCondor from '$lib/California_Condor.jpg';
  import { MapLibre,DefaultMarker } from 'svelte-maplibre';

	$: birds = [];
	let fetchDataFromBackend;
	let backendLink = 'http://172.20.10.2:8080';
	// let backendLink = 'http://localhost:8080';
	onMount(async () => {
		const fetchData = async () => {
			try {
				if (fetchDataFromBackend) {
					const response = await axios.post(`${backendLink}/birds/get`, {});
					if (response?.data?.statusCode === '000') {
						const data = response.data.data;
						if (birds.length == 0) {
							birds = data;
						} else {
							// Add only the new birds that are not already in the array
							const newBirds = await data.filter(
								(newBird) => !birds.some((existingBird) => existingBird._id === newBird._id)
							);
							birds = [...birds, ...newBirds];
							console.log('NEW BIRDS', newBirds);
							// birds = birds
						}
					} else {
						console.error('Failed to fetch data:', response.statusText);
					}
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
	<div >
		<h1 class="mt-3 h1 text-center">Bird Watcher</h1>
		<div class="mt-3 flex justify-center">
			<SlideToggle name="slider-label" active="bg-primary-500" bind:checked={fetchDataFromBackend}
				>Fetch Data</SlideToggle
			>
		</div>
					

			
		<div class="flex flex-col-reverse">
			{#each birds as bird}
				<div class="mt-6 card p-4 flex items-center flex-col" transition:fade={{ duration: 300 }}>
					<h3 class="text-center mb-3">{bird.name}</h3>
					<img src={CaliforniaCondor} alt="bird" style="width: 300px;" />
			
					
						{#if bird.long && bird.long<=90 & bird.long >=-90 &&bird.lat && bird.lat<=90 & bird.lat >=-90}
						<h5 class="mt-3 text-center">Detected at:</h5>
						<h5 class="text-center">{bird.long},{bird.lat}</h5>
						<MapLibre 
					center={[bird.long,bird.lat]}
					zoom={12}
					class="map"
					standardControls
					style="https://basemaps.cartocdn.com/gl/positron-gl-style/style.json">
					<DefaultMarker lngLat={[bird.long,bird.lat]} >
					</DefaultMarker>
				</MapLibre> 
						{/if}
							
				</div>
				
			{/each}
		</div>
	</div>
</div>

<style>
	.container {
		min-height: 100vh;
	}
	:global(.map) {
		
		height: 200px;
				width: 400px;
			}
</style>
