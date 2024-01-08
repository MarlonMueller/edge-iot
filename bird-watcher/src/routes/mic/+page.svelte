<script>
	import { onMount } from 'svelte';
	import { io } from 'socket.io-client';

	const socket = io();
	let connected;
	socket.on('connect', () => {
		console.log('Connected with socket ID: ' + socket.id);
	});
	socket.on('message', (data) => {
		console.log(data);
		if (connected) {
			player.feed(event.data);
			worker.postMessage(event.data);
		}
	});
	let connectBtn;
	let pauseBtn;
	let continueBtn;

	let player;
	let graphDataArray = Array(100).fill(0);

	onMount(() => {});

	function connect() {
		connectBtn = document.getElementById('connectBtn');
		pauseBtn = document.getElementById('pauseBtn');
		continueBtn = document.getElementById('continueBtn');

		connectBtn.disabled = false;
		pauseBtn.disabled = true;
		continueBtn.disabled = true;

		const worker = new Worker('src/routes/mic/worker.js');
		worker.addEventListener('message', (e) => {
			graphDataArray = graphDataArray.concat(e.data);
			graphDataArray.splice(0, 1);

			const dataUpdate = {
				y: [graphDataArray]
			};

			Plotly.update('graph', dataUpdate);
		});

		const layout = {
			title: 'Streaming Data',
			paper_bgcolor: '#000',
			plot_bgcolor: '#000',
			xaxis: {
				domain: [0, 1],
				showticklabels: false,
				color: '#FFF'
			},
			yaxis: {
				domain: [0, 1],
				color: '#FFF',
				rangemode: 'auto'
			}
		};

		Plotly.newPlot(
			'graph',
			[
				{
					y: graphDataArray,
					mode: 'lines',
					line: { color: '#DF56F1' }
				}
			],
			layout
		);
		connectBtn.disabled = !connectBtn.disabled;
		pauseBtn.disabled = !pauseBtn.disabled;

		player = new PCMPlayer({
			inputCodec: 'Int16',
			channels: 1,
			sampleRate: 44100
		});
		connected = true;
	}

	function changeVolume(e) {
		player.volume(document.querySelector('#range').value);
	}

	async function pause() {
		pauseBtn.disabled = true;
		continueBtn.disabled = false;
		await player.pause();
	}

	function continuePlay() {
		player.continue();
		pauseBtn.disabled = false;
		continueBtn.disabled = true;
	}
</script>

<svelte:head>
	<script src="https://cdn.plot.ly/plotly-latest.min.js" charset="utf-8"></script>
	<script src="https://unpkg.com/pcm-player"></script>
	<script type="worker.js"></script>
</svelte:head>

<div class="flex justify-center">
	<button id="connectBtn" class="btn variant-filled mt-5" on:click={connect}>Start</button>
</div>

<div class:invisible={!connected} class="mt-5">
	<input
		type="range"
		max="1"
		value="0.5"
		min="0"
		id="range"
		on:change={changeVolume}
		step="0.1"
	/><br />
	<button id="pauseBtn" class="btn variant-filled mt-5" on:click={pause}>Pause Playing</button>
	<button id="continueBtn" class="btn variant-filled mt-5" on:click={continuePlay}
		>Continue Playing</button
	>
	<div id="graph"></div>
</div>

<style>
</style>
