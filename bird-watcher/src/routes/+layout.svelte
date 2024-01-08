<script>
	import '../app.postcss';
	import { TabGroup, Tab, TabAnchor } from '@skeletonlabs/skeleton';
	import { page } from '$app/stores';
</script>

<TabGroup justify="justify-center">
	<TabAnchor href="/" selected={$page.url.pathname === '/'}>
		<span>Home</span>
	</TabAnchor>
	<TabAnchor href="/mic" selected={$page.url.pathname === '/mic'}>
		<span>Recordings</span>
	</TabAnchor>
</TabGroup>

<slot />
<!-- <script>
	import { io } from 'socket.io-client';

	const socket = io();
	$: messageFromWebsocket = '';
	socket.on('connect', () => {
		console.log('Connected with socket ID: ' + socket.id);
	});
	socket.on('message', (data) => {
		messageFromWebsocket = message;
		console.log(message);
	});
</script> -->
