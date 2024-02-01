<script>
	import { onMount } from 'svelte';
	import { fade } from 'svelte/transition';
	import { SlideToggle } from '@skeletonlabs/skeleton';
	import axios from 'axios';
	import WaterRail from '$lib/WaterRail.png';
	import CettisWarbler from '$lib/CettisWarbler.jpg';
	import CommonSandpiper from '$lib/CommonSandpiper.jpg';
	import { MapLibre, DefaultMarker, Popup } from 'svelte-maplibre';

	$: birds = [];
	$: mapCenter = [11.6654128, 48.262547];
	$: mapZoom = 13;

	let map;
	function scrollToTop() {
		map.scrollIntoView();
	}
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

<div class="container mx-auto flex justify-center" bind:this={map}>
	<div style="width: 90%;">
		<h1 class="mt-3 h1 text-center">BirdNet</h1>
		<div class="my-3 flex justify-center">
			<SlideToggle name="slider-label" active="bg-primary-500" bind:checked={fetchDataFromBackend}
				>Fetch Data</SlideToggle
			>
		</div>
		<div class="flex">
			<MapLibre
				center={mapCenter}
				zoom={mapZoom}
				class="map"
				standardControls
				style="https://basemaps.cartocdn.com/gl/positron-gl-style/style.json"
			>
				{#each birds as bird}
					{#if bird.name != 'No detection' && bird.long && (bird.long <= 90) & (bird.long >= -90) && bird.lat && (bird.lat <= 90) & (bird.lat >= -90)}
						<DefaultMarker lngLat={[bird.long, bird.lat]}>
							<Popup open={bird.open} offset={[0, -10]}>
								<div class="text-lg font-bold text-black text-center">
									{bird.name}
									{#if bird.name === 'Water Rail'}
										<img src={WaterRail} alt="bird" style="width: 200px;" />
									{:else if bird.name === "Cetti's Warbler"}
										<img src={CettisWarbler} alt="bird" style="width: 200px;" />
									{:else if bird.name === 'Common Sandpiper'}
										<img src={CommonSandpiper} alt="bird" style="width: 200px;" />
									{/if}
								</div>
							</Popup></DefaultMarker
						>
					{/if}
				{/each}
			</MapLibre>
			<div class="flex flex-col-reverse w-100 ml-auto">
				{#each birds as bird}
					<div
						class="mt-6 card p-4 flex items-center flex-col w-80"
						transition:fade={{ duration: 300 }}
					>
						<h3 class="text-center mb-3">{bird.name}</h3>
						{#if bird.name === 'Water Rail'}
							<img src={WaterRail} alt="bird" style="width: 300px;" />
						{:else if bird.name === "Cetti's Warbler"}
							<img src={CettisWarbler} alt="bird" style="width: 300px;" />
						{:else if bird.name === 'Common Sandpiper'}
							<img src={CommonSandpiper} alt="bird" style="width: 300px;" />
						{/if}

						{#if bird.long && (bird.long <= 90) & (bird.long >= -90) && bird.lat && (bird.lat <= 90) & (bird.lat >= -90)}
							{#if bird.name != 'No detection'}
								<h5 class="mt-3 text-center">Detected at: Node {bird.esp}</h5>
								<h5 class="text-center">{bird.long}, {bird.lat}</h5>
								<button
									type="button"
									class="btn variant-filled mt-2"
									on:click={() => {
										// mapZoom = 13;
										mapCenter = [bird.long, bird.lat];
										birds.map((x) => {
											if (bird._id != x.id) bird.open = false;
										});
										bird.open = true;
									}}>Find on map</button
								>
							{:else}
								<h5 class="mt-0 text-center">at:</h5>
								<h5 class="text-center">{bird.long},{bird.lat}</h5>
							{/if}
						{/if}
					</div>
				{/each}
			</div>
		</div>
	</div>
</div>

<style>
	.container {
		min-width: 100%;
	}
	:global(.map) {
		height: 70vh;
		width: 55%;
		position: fixed;
	}
</style>
