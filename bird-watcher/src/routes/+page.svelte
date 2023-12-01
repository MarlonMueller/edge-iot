<script>
	import { onMount } from "svelte";
	import { fade } from 'svelte/transition';

	$: birds=[]


	//Dummy data
	onMount(async ()=>{
		
		while(true){
			birds.push({name:"California condor", long:"48.1514606",lat:"11.5779399"})
			birds=birds
			await delay(2000);
		}
	})
	const delay = (milliseconds) => {
		return new Promise(resolve => {
			setTimeout(() => {
			resolve();
			}, milliseconds);
		});
		};
</script>
<div class="container mx-auto flex justify-center">
	<div class="space-y-10">
		<h1 class="h1 text-center">Bird Watcher</h1>
		<div class="flex flex-col-reverse">
			{#each birds as bird}
			<div class="mt-10 card p-4" transition:fade={{  duration: 300 }}>

				<h3 class='text-center mb-3'>{bird.name}</h3>
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