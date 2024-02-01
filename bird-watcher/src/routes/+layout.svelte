<script>
	import '../app.postcss';
	import { TabGroup, Tab, TabAnchor } from '@skeletonlabs/skeleton';
	import { page } from '$app/stores';
</script>

<!-- <TabGroup justify="justify-center">
	<TabAnchor href="/" selected={$page.url.pathname === '/'}>
		<span>Home</span>
	</TabAnchor>
	<TabAnchor href="/mic" selected={$page.url.pathname === '/mic'}>
		<span>Recordings</span>
	</TabAnchor>
</TabGroup> -->

<slot />
